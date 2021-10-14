import json
from flask import Flask, request, jsonify
from flask_script import Manager, Server
import requests
import nltk
import tweepy
from nltk.sentiment.vader import SentimentIntensityAnalyzer
import pandas as pd
import re
from dotenv import load_dotenv
import os


def authenticate():
    # Authentication
    consumerKey = os.getenv("consumerKey")
    consumerSecret = os.getenv("consumerSecret")
    accessToken = os.getenv("accessToken")
    accessTokenSecret = os.getenv("accessTokenSecret")

    auth = tweepy.OAuthHandler(consumerKey, consumerSecret)
    auth.set_access_token(accessToken, accessTokenSecret)
    api = tweepy.API(auth)

    return api

def sentimentAnalysisInitialize():
    nltk.download('vader_lexicon')
    nltk.download('stopwords')

class CustomServer(Server):
    def __call__(self, app, *args, **kwargs):
        load_dotenv()
        sentimentAnalysisInitialize()
        return Server.__call__(self, app, *args, **kwargs)

def count_sentiment(data, feature):
    total = data.loc[:, feature].value_counts(dropna=False)
    percentage = round(data.loc[:, feature].value_counts(dropna=False, normalize=True)*100, 2)
    return total, percentage


app = Flask(__name__)
manager = Manager(app)

manager.add_command('runserver', CustomServer())

@app.route('/sentiment')
def sentimentAnalysis(keyword='#amtrack', noOfTweet=4000):
    tweets = tweepy.Cursor(authenticate().search, q=keyword, result_type='mixed').items(
        noOfTweet)  # noOfTweet limited by Twitter constraints
    tw_list = []

    for tweet in tweets:
        tw_list.append(tweet.text)

    tw_list = pd.DataFrame(tw_list)
    tw_list.drop_duplicates(inplace=True)
    tw_list["text"] = tw_list[0]

    remove_rt = lambda x: re.sub('RT @\w+: ', " ", x)
    rt = lambda x: re.sub("(@[A-Za-z0–9]+)|([^0-9A-Za-z \t])|(\w+:\/\/\S+)", " ", x)
    tw_list["text"] = tw_list.text.map(remove_rt).map(rt)
    tw_list["text"] = tw_list.text.str.lower()

    for index, row in tw_list['text'].iteritems():
        score = SentimentIntensityAnalyzer().polarity_scores(row)
        neg = score['neg']
        neu = score['neu']
        pos = score['pos']

        if neg > pos:
            tw_list.loc[index, 'sentiment'] = "negative"

        elif pos > neg:
            tw_list.loc[index, 'sentiment'] = "positive"

        else:
            tw_list.loc[index, 'sentiment'] = "neutral"

        tw_list.loc[index, 'neg'] = neg
        tw_list.loc[index, 'neu'] = neu
        tw_list.loc[index, 'pos'] = pos

    return jsonify(
        keyword=keyword,
        numTweets=tw_list.shape[0],
        positive=count_sentiment(tw_list, "sentiment")[1]["positive"],
        negative=count_sentiment(tw_list, "sentiment")[1]["negative"],
        neutral=count_sentiment(tw_list, "sentiment")[1]["neutral"]
    )

@app.route('/sensors')
def readSensors():
    id = request.args.get('ID')
    ip = searchIpFromId(id)
    if ip is not None:
        return requests.get(ip+'/all/'+id).json()   #aggiunto per poter gestire più treni sullo stesso dispositivo
    return None


def searchIpFromId(id):
    ipList = os.getenv("ipList").split(", ")
    idx = 0
    for idx, address in enumerate(ipList):
        r = requests.get(address + '/id')
        if r.text.replace("\r\n", "") == id:
            break
        else:
            idx = -1
    if idx == -1:
        return None
    return ipList[idx]


@app.route('/trains')
def listTrains():
    ipList = os.getenv("ipList").split(", ")
    idList = []
    for address in ipList:
        print(address)
        idList.append(requests.get(address+'/id').text.replace("\r\n", ""))
    return json.dumps(idList)

if __name__ == '__main__':
    app.run()

