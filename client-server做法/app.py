from flask import Flask, render_template, request, session, redirect
from flask_mysqldb import MySQL
from datetime import datetime
import hashlib ,serial

app = Flask(__name__)
arduino = serial.Serial('/dev/ttyACM0', 9600)
app.secret_key = 'Aa123456'  

app.config['MYSQL_HOST'] = 'localhost'  
app.config['MYSQL_USER'] = 'root'  
app.config['MYSQL_PASSWORD'] = 'Aa123456'  
app.config['MYSQL_DB'] = 'users'  

mysql = MySQL(app)

@app.route('/')
def index():
    if 'username' in session:
        return render_template('index.html', username=session['username'])
    else:
        return redirect('/login')
        
@app.route('/led/on')
def led_on():
    arduino.write(b'1') 
    
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    
   
    insert_query = "INSERT INTO button_history (button_name, action, timestamp) VALUES (%s, %s, %s)"
    values = ('LED', 'turned on', timestamp)
    
    cursor = mysql.connection.cursor()
    cursor.execute(insert_query, values)
    mysql.connection.commit()
    cursor.close() 
    
    return 'LED turned on'

@app.route('/led/off')
def led_off():
    arduino.write(b'0')
    
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    insert_query = "INSERT INTO button_history (button_name, action, timestamp) VALUES (%s, %s, %s)"
    values = ('LED', 'turned off', timestamp)
    
    cursor = mysql.connection.cursor()
    cursor.execute(insert_query, values)
    mysql.connection.commit()
    cursor.close() 
      
    return 'LED turned off'

@app.route('/led/brightness/up')
def brightness_up():
    arduino.write(b'+') 
     
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    insert_query = "INSERT INTO button_history (button_name, action, timestamp) VALUES (%s, %s, %s)"
    values = ('LED', 'Brightness increased', timestamp)
    
    cursor = mysql.connection.cursor()
    cursor.execute(insert_query, values)
    mysql.connection.commit()
    cursor.close() 
    
    return 'Brightness increased'

@app.route('/led/brightness/down')
def brightness_down():
    arduino.write(b'-') 
     
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    insert_query = "INSERT INTO button_history (button_name, action, timestamp) VALUES (%s, %s, %s)"
    values = ('LED', 'Brightness decreased', timestamp)
    
    cursor = mysql.connection.cursor()
    cursor.execute(insert_query, values)
    mysql.connection.commit()
    cursor.close() 
    
    return 'Brightness decreased'
    
@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']

        cursor = mysql.connection.cursor()
        cursor.execute("SELECT * FROM users WHERE username = %s", (username,))
        user = cursor.fetchone()
        cursor.close()

        if user:
            hashed_password = hashlib.sha256(password.encode()).hexdigest()
            if hashed_password == user[2]:
                session['username'] = user[1]
                return redirect('/')
            else:
                return render_template('login.html', error='密碼錯誤')
        else:
            return render_template('login.html', error='帳號不存在')

    return render_template('login.html')

@app.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']

        cursor = mysql.connection.cursor()
        cursor.execute("SELECT * FROM users WHERE username = %s", (username,))
        user = cursor.fetchone()

        if user:
            return render_template('register.html', error='帳號已存在')
        else:
            hashed_password = hashlib.sha256(password.encode()).hexdigest()
            cursor.execute("INSERT INTO users (username, password) VALUES (%s, %s)",
                           (username, hashed_password))
            mysql.connection.commit()
            cursor.close()

            session['username'] = username
            return redirect('/')

    return render_template('register.html')

@app.route('/logout')
def logout():
    session.pop('username', None)
    return redirect('/login')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
