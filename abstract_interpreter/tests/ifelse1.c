int a;

void main() {
  /*!npk a between 0 and 1 */
  if(a == 0) {
    a = 10;
  }
  else {
    a = 11;
  }
  assert(a >= 10);
  assert(a <= 11);
}
