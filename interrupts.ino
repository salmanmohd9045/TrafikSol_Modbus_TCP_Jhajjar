void isr_timer() {
  windSpeed = Rotations * 0.9;
  Rotations = 0;
}

void isr_rotation() {
  if ((millis() - ContactBounceTime) > 15 ) {
    Rotations++;
    ContactBounceTime = millis();
  }
}
