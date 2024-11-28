/*******************************************************************/
/* Cas d'etudes pour le projet du cours d'interpretation abstraite */
/* Ecrit par Olivier Bouissou (olivier.bouissou@cea.fr)            */
/* Le but de ces cas d'etudes est de vous permettre de tester      */
/* votre projet sur des exemples de programmes contenant chacun    */
/* une difficulte que vous devriez rencontrer.                     */
/*******************************************************************/
/* Boucles imbriquees 3.                                           */
/* On cherche ici a tester l'iterateur et le calcul de point fixe. */
/* Dans ces exemples, des techniques de widening intelligentes     */
/* seront necessaires pour obtenir des resultats precis.           */
/*******************************************************************/

int i;
  int j;


void main() 
{
  i=1;
  while (i<100) 
  {
    j=100;
    while (j>1)
      j = j-i;
    i=i+1;
  }
}
