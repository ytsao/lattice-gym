int a;
int b;

void main() {
  /*!npk a between 2 and 4 */
  b = (a * a) / a;
  // Note the loss of precision with interval analysis.
  assert(b >= 1);
  assert(b <= 8);
}
