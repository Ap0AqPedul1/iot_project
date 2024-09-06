import requests

# URL endpoint login Flask
url = 'http://127.0.0.1:5000/'

# Data login
login_data = {
    'email': 'user1@gmail.com',
    'password': ''
}

def login():
    response = requests.post(f"{url}login", json=login_data)

    if response.status_code == 200:
        result = response.json()
        print('Login berhasil!')
        print('User ID:', result.get('user_id'))
        return result.get('user_id')
    else:
        print('Login gagal!')
        print('Error:', response.json().get('message'))
        return None
    

def get_data():
    response = requests.get(f'{url}data')

    if response.status_code == 200:
        data = response.json()
        return data
    else:
        print('Gagal mengambil data!')
        print('Error:', response.json().get('message'))
        return None
    
def get_user_data(user_id):
    data = f'{url}/data/{user_id}'
    response = requests.get(data)
    if response.status_code == 200:
        data = response.json()
        print('Data pengguna:', data)
        return data
    else:
        print('Gagal mengambil data:', response.json().get('message'))
        return None
    
def get_name(user_id,name):
    data = f'{url}/data/{user_id}/{name}'
    response = requests.get(data)
    if response.status_code == 200:
        data = response.json()
        print('Data pengguna:', data)
        return data
    else:
        print('Gagal mengambil data:', response.json().get('message'))
        return None


