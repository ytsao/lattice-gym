/*******************************************************************/
/* Cas d'etudes pour le projet du cours d'interpretation abstraite */
/* Ecrit par Olivier Bouissou (olivier.bouissou@cea.fr)            */
/* Le but de ces cas d'etudes est de vous permettre de tester      */
/* votre projet sur des exemples de programmes contenant chacun    */
/* une difficulte que vous devriez rencontrer.                     */
/*******************************************************************/
/* Boucle 2.                                                       */
/* On cherche ici a tester l'iterateur et le calcul de point fixe. */
/* Dans ces exemples, des techniques de widening intelligentes     */
/* seront necessaires pour obtenir des resultats precis.           */
/*******************************************************************/

int i, j, k, l, m;

void main() {
  i = 0;
  j = 100;
  k = 1000;
  l = 10000;
  m = 100000;
  while (i < 1000) 
    i = i+1;
  while (j  < 1000)
    j = j+k;
  while (k > 100)
    k = k-j;
  while (l > 1000)
    l = l+k;
  while (m > 1)
    m = m-l;   
}

