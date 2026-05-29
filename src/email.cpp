#include "email.h"
#include <ESP_Mail_Client.h>
#include "secrets.h"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

SMTPSession smtp;

void smtpCallback(SMTP_Status status)
{
  Serial.println(status.info());
}

void initEmail()
{
  smtp.debug(1);
}

bool sendDailyEmail(int highest, int lowest, float average, int changes)
{
  ESP_Mail_Session session;
  SMTP_Message message;

  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;

  session.login.email = EMAIL_USER;
  session.login.password = EMAIL_APP_PASSWORD;
  session.login.user_domain = "";

  session.secure.startTLS = false;

  message.sender.name = "Tank Monitor";
  message.sender.email = EMAIL_USER;

  message.subject = "Water Tank Daily Report";

  message.addRecipient("User", RECIPIENT_EMAIL);

  String body =
    "Daily Water Tank Report\n\n"
    "Highest: " + String(highest) + "%\n" +
    "Lowest: " + String(lowest) + "%\n" +
    "Average: " + String(average) + "%\n" +
    "Changes: " + String(changes) + "\n";

  message.text.content = body.c_str();
  message.text.charSet = "utf-8";
  message.text.transfer_encoding =
    Content_Transfer_Encoding::enc_7bit;

  smtp.callback(smtpCallback);

  Serial.println("SMTP connecting...");

  if (!smtp.connect(&session))
  {
    Serial.print("SMTP connect failed: ");
    Serial.println(smtp.errorReason());
    return false;
  }

  Serial.println("SMTP connected");

  if (!MailClient.sendMail(&smtp, &message))
  {
    Serial.print("MAIL SEND FAILED: ");
    Serial.println(smtp.errorReason());
    return false;
  }

  Serial.println("MAIL SENT SUCCESSFULLY");

  return true;
}