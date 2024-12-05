int a;
int b;

void main() {
  /*!npk a between 0 and 2 */
  b = a + a;
  // NOTE: Interval analysis is not capable of catching `b > a` for instance.
  assert(b >= 0);
  assert(b <= 4);
}
