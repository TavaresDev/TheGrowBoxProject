void ldrFunction() {
    /*    moisture variable control

    IndoorNormal Ligth = 430 to 440
    tipFinger < 130
    Takes about 5 sec to normalize the value

    normal bulb 9 w about 20 cm = 971 256%

    persentage 0% = air dry, 100% = water

    */
    int lSensor = analogRead(ldrSensorPin);
    int ldrPercent = map(lSensor, 130, 450, 0, 100);
    gLdr = ldrPercent;

    Serial.print("LDR Sensor: ");
    Serial.print(lSensor); //print the value to serial port
    Serial.print(" "); 
    Serial.print(ldrPercent); //print the value to serial port
    Serial.println("%"); 
    delay(1000);
}
