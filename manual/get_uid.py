from library import *

if __name__ == '__main__':
    user_id = login()
    if user_id:
        print(f'User ID setelah login: {user_id}')
