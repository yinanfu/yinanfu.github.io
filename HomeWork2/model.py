import pandas as pd
from sklearn.linear_model import LinearRegression
import joblib


def train():
    df = pd.read_csv("SBP.csv")

    x = df[["Age", "Weight"]]
    y = df["SBP"]

    regr = LinearRegression()
    regr.fit(x, y)

    joblib.dump(regr, "regr.pkl")


def load(x,y):

    clf = joblib.load("/home/YinanFu/mysite/regr.pkl")
    age = x
    weight = y
    x = pd.DataFrame([[age, weight]], columns=["Age", "Weight"])
    prediction = clf.predict(x)[0]
    # print(prediction)
    return prediction


if __name__ == "__main__":

    train()
    load()
