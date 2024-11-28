/*******************************************************************/
/* Cas d'etudes pour le projet du cours d'interpretation abstraite */
/* Ecrit par Olivier Bouissou (olivier.bouissou@cea.fr)            */
/* Le but de ces cas d'etudes est de vous permettre de tester      */
/* votre projet sur des exemples de programmes contenant chacun    */
/* une difficulte que vous devriez rencontrer.                     */
/*******************************************************************/
/* Boucle 0.                                                       */
/* On cherche ici a tester l'iterateur et le calcul de point fixe. */
/* Dans ces exemples, des techniques de widening intelligentes     */
/* seront necessaires pour obtenir des resultats precis.           */
/*******************************************************************/

int x;


void main() 
{
  x = 0;
  while (x<100)
    x++;
}

