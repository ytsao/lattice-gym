/*******************************************************************/
/* Cas d'etudes pour le projet du cours d'interpretation abstraite */
/* Ecrit par Olivier Bouissou (olivier.bouissou@cea.fr)            */
/* Le but de ces cas d'etudes est de vous permettre de tester      */
/* votre projet sur des exemples de programmes contenant chacun    */
/* une difficulte que vous devriez rencontrer.                     */
/*******************************************************************/
/* Boucles imbriquees 1.                                           */
/* On cherche ici a tester l'iterateur et le calcul de point fixe. */
/* Dans ces exemples, des techniques de widening intelligentes     */
/* seront necessaires pour obtenir des resultats precis.           */
/*******************************************************************/

int i;
int j;
int k;
int l;


void main()
{

  i = 0;
  k = 9;
  j = -100;
  l = 0;
  while (l <= 1000) {
    while (i <= 100) 
      {
	i = i + 1; 
	while (j < 20) 
	  j = i+j; 
	k = 4; 
	while (k <=3) 
	  k = k+1; 
      } 
    l = l+j;
  }
}



