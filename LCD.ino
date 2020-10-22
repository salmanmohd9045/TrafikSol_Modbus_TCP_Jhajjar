long int menuTime = millis();

void deviceSettings() {
  bool loopExit = false;
  while (true) {
    String menu[] = {F("MENU"), F("Settings"), F("Product Info")};
    uint16_t choice = displayMenu(menu, ArraySize(menu));
    switch (choice) {
      case 1: {
          String menu[] = {F("Settings"), F("View IP Addr"), F("Configure IP")};
          uint16_t choice = displayMenu(menu, ArraySize(menu));
          switch (choice) {
            case 1:
              viewIPAddress();
              break;

            case 2:
              configureIPAddress();
              break;

            case 99: {
                while (!digitalRead(EXIT)) {}
                break;
              }
          }
        }
        break;

      case 2: {
          lcd.clear();
          lcd.clear();
          lcd.print("WMS - Modbus");
          lcd.setCursor(0, 1);
          lcd.print("VER: " + String(SOFT_VERSION));
          delay(5000);
          break;
        }

      case 99: {
          loopExit =  true;
          while (!digitalRead(EXIT)) {}
          break;
        }
    }

    if (loopExit) {
      break;
    }
  }
}

void viewIPAddress() {
  lcd.clear();
  lcd.print("IP Address:");
  lcd.setCursor(0, 1);
  lcd.print(ip_array[0]);
  lcd.print(".");
  lcd.print(ip_array[1]);
  lcd.print(".");
  lcd.print(ip_array[2]);
  lcd.print(".");
  lcd.print(ip_array[3]);
  while (true) {
    delay(10);
    if (!digitalRead(EXIT)) {
      while (!digitalRead(EXIT)) {}
      break;
    }
  }
}

void configureIPAddress() {
  bool updateStatus = false, sign = true;
  uint8_t arr[15] = {ip_array[0] / 100, (ip_array[0] % 100) / 10, ip_array[0] % 10, 0,
                     ip_array[1] / 100, (ip_array[1] % 100) / 10, ip_array[1] % 10, 0,
                     ip_array[2] / 100, (ip_array[2] % 100) / 10, ip_array[2] % 10, 0,
                     ip_array[3] / 100, (ip_array[3] % 100) / 10, ip_array[3] % 10
                    };
  uint8_t cursorKey = 0, keyValue = arr[0];
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Configure IP:"));
  lcd.setCursor(0, 1);
  for (int cKey = 0; cKey < 15; cKey++) {
    if (cKey == 3 | cKey == 7 | cKey == 11) lcd.print(".");
    else lcd.print(arr[cKey]);
  }
  while (!digitalRead(ENTER)) {}
  do {
    lcd.setCursor(cursorKey, 1);
    lcd.blink();
    delay(200);
    if (!digitalRead(NEXT)) {
      keyValue++;
      if (keyValue > 2) {
        if (cursorKey == 0 | cursorKey == 4 | cursorKey == 8 | cursorKey == 12)
          keyValue = 0;
        if (keyValue > 9) keyValue = 0;
      }
      arr[cursorKey] = keyValue;
      lcd.print(keyValue);
    }
    if (!digitalRead(MENU)) {
      cursorKey++;
      if (cursorKey == 15) cursorKey = 0;
      if (cursorKey == 3 | cursorKey == 7 | cursorKey == 11) cursorKey++;
      keyValue = arr[cursorKey];
    }
    if (!digitalRead(ENTER)) {
      updateStatus = true;
      break;
    }

    if (updateStatus) {
      break;
    }
    lcd.noBlink();
  } while (digitalRead(EXIT));
  lcd.noBlink();
  if (updateStatus) {
    ip_array[0] = arr[0] * 100 + arr[1] * 10 + arr[2];
    ip_array[1] = arr[4] * 100 + arr[5] * 10 + arr[6];
    ip_array[2] = arr[8] * 100 + arr[9] * 10 + arr[10];
    ip_array[3] = arr[12] * 100 + arr[13] * 10 + arr[14];
    EEPROM.write(IP_ADDR, ip_array[0]);
    EEPROM.write(IP_ADDR + 1, ip_array[1]);
    EEPROM.write(IP_ADDR + 2, ip_array[2]);
    EEPROM.write(IP_ADDR + 3, ip_array[3]);
    for (int  i = 0; i < 4; i++)
      Serial.println(ip_array[i]);
    dialogMsg(F("IP Address"), F("Configured"));
  } else dialogMsg(F("IP Address"), F(" Not Configured"));
}

int displayMenu(String *displayItems, uint16_t n) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(displayItems[0]);
  lcd.setCursor(0, 1);
  lcd.print(">");

  if (!digitalRead(ENTER)) {}
  uint8_t i = 1;
  menuTime = millis();
  do {
    lcd.setCursor(2, 1);
    lcd.print(displayItems[i]);

    if (!digitalRead(NEXT)) {
      menuTime = millis();
      i++;
      if (i == n) i = 1;
      lcd.setCursor(2, 1);
      lcd.print("              ");
      delay(50);
    }

    if (!digitalRead(EXIT)) {
      while (!digitalRead(EXIT)) {}
      return 99;
    }

    if (millis() - menuTime > 15000) {
      return 99;
    }

    delay(200);
  } while (digitalRead(ENTER));
  delay(50);
  return i;
}

void dialogMsg(String header, String msg) {
  uint8_t l;
  lcd.clear();
  l = header.length();
  if (l > 16)
    header.substring(0, 16);
  header = "<" + header + ">";
  l = header.length();
  lcd.setCursor(8 - l / 2, 0);
  lcd.print(header);
  l = msg.length();
  if (l > 16)
    msg.substring(0, 16);
  lcd.setCursor(8 - l / 2, 1);
  lcd.print(msg);
  delay(2500);
}

void dialogMsg(String header, String msg, int _time) {
  uint8_t l;
  lcd.clear();
  l = header.length();
  if (l > 16)
    header.substring(0, 16);
  header = "<" + header + ">";
  l = header.length();
  lcd.setCursor(8 - l / 2, 0);
  lcd.print(header);
  l = msg.length();
  if (l > 16)
    msg.substring(0, 16);
  lcd.setCursor(8 - l / 2, 1);
  lcd.print(msg);
  delay(_time);
}
