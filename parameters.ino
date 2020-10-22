void getParameterValues() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  windDirection = getWindDirection();
  surfaceTemperature = readSurfaceTemperature();
  visibility = readVisibility();
  arr_str[0] = String(temperature);
  arr_str[1] = String(humidity);
  arr_str[2] = String(windDirection);
  arr_str[3] = String(windSpeed);
  arr_str[4] = String(surfaceTemperature);
  arr_str[5] = String(visibility);
}

uint16_t getWindDirection() {
  int Direction = map(analogRead(WindVanePin), 0, 1023, 0, 360);

  if (Direction > 360) Direction = Direction - 360;
  if (Direction < 0)  Direction = Direction + 360;
  if (abs(Direction - lastDirectionValue) >= 5) {
    lastDirectionValue = Direction;
  }
  return lastDirectionValue;
}

float readSurfaceTemperature() {
  //  double sumValue = 0;
  //  for (int i = 0; i < 100; i++) {
  //    sumValue += analogRead(surfTempPin);
  //  }
  //  sumValue /= 100;
  //  sumValue = sumValue / 1023.0 * 5;
  //  float value = 104 * sumValue - 18;
  //  if (value < 0) return 0;
  //  else return value;

  Vo = analogRead(surfTempPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2 ));
  Tc = T - 273.15;
  return Tc;
}

int readVisibility() {
  SerialVis.write(ask, 8);
  long int _time = millis();
  byte val[10];
  int i = 0;
  int vis = 2500;
  while (millis() - _time < 1000) {
    if (SerialVis.available()) {
      val[i++] = SerialVis.read();
      //      Serial.println(val[i - 1]);

    }
  }
  vis = val[3] * 255 + val[4];
  if (vis > 3000) vis = 3000;
  return vis;
}

