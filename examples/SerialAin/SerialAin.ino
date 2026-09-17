void setup(void)
{
    uart1_begin(9600);
    pinMode(18, OUTPUT);
}

void loop(void)
{
    int v = analogRead(A0);
    uart1_print("A0=");
    uart1_print_int(v);
    analogWrite(18, 100);
    delay(200);
}
