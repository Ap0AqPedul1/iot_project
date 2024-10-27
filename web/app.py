from config import *

app = Flask(__name__)

@app.route('/')
def home():
    return "Hello, Azhari Bastomi!"

@app.route('/login', methods=['POST'])
def login_user():
    data = request.json
    email = data.get('email')
    password = data.get('password')

    try:
        # Login dengan email dan password
        user = auth.get_user_by_email(email)
        user_id = user.uid
        return jsonify({'status': 'success', 'user_id': user_id}), 200
    except firebase_admin.auth.AuthError as e:
        return jsonify({'status': 'error', 'message': str(e)}), 400

@app.route('/data', methods=['GET'])
@app.route('/data/<user_id>', methods=['GET'])
@app.route('/data/<user_id>/<name>', methods=['GET'])
def get_data(user_id=None, name=None):
    try:
        data = ref.get()
        if user_id:
            user_data = data.get(user_id, 'Data tidak ditemukan')
            if name:
                list_data = user_data.get(name, 'Data tidak ditemukan')
                return jsonify(list_data), 200
            return jsonify(user_data), 200
        return jsonify(data), 200
    except Exception as e:
        return jsonify({
            'message': 'Gagal mengambil data',
            'error': str(e)
        }), 500

@app.route('/update_id/<string:user_id>/<string:name>', methods=['POST'])
def update_id(user_id, name):
    # Referensi ke semua produk
    ref = db.reference('products')

    # Ambil data untuk memeriksa panjang ID
    data = ref.child(user_id).child(name).get()  # Mengambil data berdasarkan category_id dan item_id
    if data is None:
        return jsonify({'error': 'Data tidak ditemukan'}), 404

    current_id = data.get('id')  # Ambil ID saat ini

    # Cek panjang ID
    if current_id and len(current_id) > 10:  # Pastikan ID tidak None
        # Update ID
        new_id = request.json.get('new_id', 'newID123')  # Ambil ID baru dari body permintaan
        ref.child(user_id).child(name).update({'id': new_id})  # Update ID di lokasi category_id/item_id
        return jsonify({'message': 'ID berhasil diperbarui.', 'new_id': new_id}), 200
    else:
        return jsonify({'message': 'Panjang ID saat ini tidak sama dengan 20, tidak ada pembaruan yang dilakukan.', 'current_id_length': len(current_id) if current_id else 'None'}), 400


@app.route('/update', methods=['POST'])
def update_data():
    current_data = ref.get()
    name = request.json.get('name')
    if name in current_data:
        # Update data jika nama sudah ada
        ref.child(name).update({
            'activation': request.json.get('activation'),
            'status': request.json.get('status'),
            'state': request.json.get('state'),
            'feed': request.json.get('feed'),
            'servo': request.json.get('servo')
        })
        return jsonify({"message": "Data updated successfully"}), 200
    else:
        # Tambah data baru jika nama belum ada
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
        return jsonify({"message": "Data added successfully"}), 200

@app.route('/search/<name>', methods=['GET'])
def search(name):
    result = ref.child(name).get()
    if result:
        return jsonify(result), 200
    else:
        return jsonify({"message": "Data not found"}), 404

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
        variable = request.json.get('variable')
        new_data = request.json.get('new_data')
        ref.child(name).update({variable: new_data})
        return jsonify({"message": "Data updated successfully"}), 200
    else:
        return jsonify({"message": "Data not found"}), 404

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')


