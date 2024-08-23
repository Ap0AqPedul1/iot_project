from config import *

app = Flask(__name__)

@app.route('/update', methods=['POST'])
def update_data():
    current_data = ref.get()
    name = request.json.get('name')
    if name in current_data:
        return jsonify({"message": "Nama sudah ada"}), 400
    else:
        user_data = {
            name: {
                'activation': request.json.get('activation'),
                'status': request.json.get('status'),
                'state': request.json.get('state'),
                'feed': request.json.get('feed'),
                'servo': request.json.get('servo')
            }
        }
        ref.update(user_data)
        return jsonify({"message": "Data updated successfully"}), 200

@app.route('/search/<name>', methods=['GET'])
def search(name):
    result = ref.child(name).get()
    if result:
        return jsonify(result), 200
    else:
        return jsonify({"message": "Data not found"}), 404

@app.route('/data', methods=['GET'])
def get_all_data():
    data = ref.get()
    return jsonify(data), 200

@app.route('/delete/<name>', methods=['DELETE'])
def delete_data(name):
    current_data = ref.get()
    if name in current_data:
        ref.child(name).delete()
        return jsonify({"message": f"Component with name {name} has been deleted."}), 200
    else:
        return jsonify({"message": f"Component with name {name} does not exist. No action taken."}), 404

@app.route('/update/<name>', methods=['PUT'])
def update_(name):
    current_data = ref.get()
    if name in current_data:
        viariable = request.json.get('variable')
        new_data = request.json.get('new_data')
        ref.child(name).update({viariable: new_data})
        return jsonify({"message": "Data updated successfully"}), 200
    else:
        return jsonify({"message": "Data not found"}), 404

if __name__ == '__main__':
    app.run(debug=True)
