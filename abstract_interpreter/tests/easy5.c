int a;
int b;

void main() {
  /*!npk a between 0 and 2 */
  b = (a + a) * 2;
  assert(b >= 0);
  assert(b <= 8);
}
