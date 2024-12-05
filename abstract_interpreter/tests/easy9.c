int a, b, c, d, e;

void main() {
  a = 23;
  b = 5;
  c = a + b;
  d = a + c;
  e = d + c;
  assert(e == 79);
}
