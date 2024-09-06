import firebase_admin
from firebase_admin import credentials, auth

# Inisialisasi aplikasi Firebase dengan file kunci layanan
cred = credentials.Certificate('serviceAccountKey.json')
firebase_admin.initialize_app(cred)

def login_user(email, password):
    try:
        # Login dengan email dan password
        user = auth.get_user_by_email(email)
        print('User berhasil login')
        print('User ID:', user.uid)  # Mengambil User ID
        return user.uid
    except firebase_admin.auth.AuthError as e:
        print('Login gagal:', e)
        return None

# Contoh login
email = 'user1@gmail.com'
password = ''
user_id = login_user(email, password)

if user_id:
    print('User ID:', user_id)
