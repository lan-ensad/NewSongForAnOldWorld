//-------------------------------------
//              LEDS
void Snake(int up, int R, int G, int B, boolean Rdm) {
  pixels.clear();
  if (millis() - prevMillisSnake >= up) {
    prevMillisSnake = millis();
    if (!Rdm) {
      pixels.setPixelColor(pix, pixels.Color(R, G, B));
      pixels.show();
      (pix >= NUMPIXELS) ? pix = 0 : pix++;
    } else {
      pixels.setPixelColor(pix, pixels.Color(random(255), random(255), random(255)));
      pixels.show();
      (pix >= NUMPIXELS) ? pix = 0 : pix++;
    }
  } else {
    pixels.clear();
  }
}
void Flash(int up, int R, int G, int B, boolean Rdm) {
  pixels.clear();
  
  if (millis() - prevMillisFlash >= up) {
    prevMillisFlash = millis();
    if (flashIsUp) {
      pixels.clear();
      flashIsUp = false;
    } else {
      if (!Rdm) {
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(R, G, B));
        }
      } else {
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(random(255), random(255), random(255)));
        }
      }
      flashIsUp = true;
    }
    pixels.show();
  }
}