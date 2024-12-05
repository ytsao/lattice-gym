int a;
int b;

void main() {
  /*!npk b between 0 and 1 */
  if(b == 0) {
    a = 1;
  }
  else {
    a = 2;
  }
  assert(a <= 2);
  assert(a >= 1);
}
