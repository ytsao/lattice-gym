/*******************************************************************/
/* Cas d'etudes pour le projet du cours d'interpretation abstraite */
/* Ecrit par Olivier Bouissou (olivier.bouissou@cea.fr)            */
/* Le but de ces cas d'etudes est de vous permettre de tester      */
/* votre projet sur des exemples de programmes contenant chacun    */
/* une difficulte que vous devriez rencontrer.                     */
/*******************************************************************/
/* Boucles imbriquees 2.                                           */
/* On cherche ici a tester l'iterateur et le calcul de point fixe. */
/* Dans ces exemples, des techniques de widening intelligentes     */
/* seront necessaires pour obtenir des resultats precis.           */
/*******************************************************************/


int i;
  int j;
  int k;


void main() {
  i = 0;
  j = 0;
  k = 0;
  while (i < 100) {
    while (j < 100) {
      while (k < 100) {
	i=i+1;
	j=j+1;
	k=k+1;
      }
    }
  }
}
