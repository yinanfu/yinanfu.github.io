from flask import Flask, render_template,request,redirect
from model import load
app = Flask(__name__)


@app.route('/')
def hello():
    bp = ""
    return render_template('index.html',bp=bp)


@app.route('/' ,methods=['POST'])
def form():
    age = request.form['age']
    Weight = request.form['weight']
    bp = load(age,Weight)
    print(bp)
    return render_template('index.html',bp= bp)
