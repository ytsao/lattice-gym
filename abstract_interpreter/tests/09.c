int x, y, i;

void main() {
  x = 1;
  i = 0;
  y = x+x;

  while(i<10) {
    
    x++;
    y = x+x;
    x = x+(-1);
    i++;

  }

}
