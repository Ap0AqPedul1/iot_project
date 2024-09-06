from flask import Flask, request, jsonify
import firebase_admin
from firebase_admin import credentials, db, auth


api = 'https://remote-app-cd2d1-default-rtdb.asia-southeast1.firebasedatabase.app/'
file = 'serviceAccountKey.json'
key = "products"
cred = credentials.Certificate(file)
firebase_admin.initialize_app(cred, {
    'databaseURL': api
})

ref = db.reference(key)

component = {
    'name'      : 'apo',
    'activation': 0,
    'status'    : 'kosong',
    'state'     : 1,
    'feed'      : 30.1,
    'servo'     : 120
}
