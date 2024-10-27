import firebase_admin
from firebase_admin import credentials, auth, db

# Inisialisasi aplikasi Firebase dengan file kunci layanan
cred = credentials.Certificate('serviceAccountKey.json')
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://remote-app-cd2d1-default-rtdb.asia-southeast1.firebasedatabase.app/'
})
# Referensi ke lokasi data
ref = db.reference('products/rOCBwJVW9QUzKGylMj7RSYz7fif1/1234')

# Ambil data untuk memeriksa panjang ID
data = ref.get()
print(data)
current_id = data.get('id')  # Ambil ID saat ini

# Cek panjang ID
if current_id and len(current_id) == 20:  # Pastikan ID tidak None
    # Update ID
    new_id = "newID123"  # Ganti dengan ID baru yang diinginkan
    ref.update({
        'id': new_id
    })
    print("ID berhasil diperbarui.")
else:
    print("Panjang ID saat ini tidak sama dengan 20, tidak ada pembaruan yang dilakukan.")