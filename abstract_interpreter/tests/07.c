/*******************************************************************/
/* Cas d'etudes pour le projet du cours d'interpretation abstraite */
/* Ecrit par Olivier Bouissou (olivier.bouissou@cea.fr)            */
/* Le but de ces cas d'etudes est de vous permettre de tester      */
/* votre projet sur des exemples de programmes contenant chacun    */
/* une difficulte que vous devriez rencontrer.                     */
/*******************************************************************/
/* Branchement conditionnel 2. Expression dans les tests.          */
/* On cherche ici a tester les fonctions de transfert liees aux    */
/* test abstraits. On doit avoir, a la fin, t=1.                   */
/*******************************************************************/


int x,y,z,t;

void main() {

  /*!npk x between 1 and 5 */
  /*!npk y between 2 and 10 */

	
	if ( x < y)
		z=x*x;
	else
		z=y*y;
	
	if (z<=x*y)
		t=1;
	else 
		t=0;
}
