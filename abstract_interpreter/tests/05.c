/*******************************************************************/
/* Cas d'etudes pour le projet du cours d'interpretation abstraite */
/* Ecrit par Olivier Bouissou (olivier.bouissou@cea.fr)            */
/* Le but de ces cas d'etudes est de vous permettre de tester      */
/* votre projet sur des exemples de programmes contenant chacun    */
/* une difficulte que vous devriez rencontrer.                     */
/*******************************************************************/
/* Branchement conditionnel 0. Valeur absolue.                     */
/* On cherche ici a tester les fonctions de transfert liees aux    */
/* test abstraits. On doit pouvoir prouver que y est positif en    */
/* en sortie du programme.                                         */
/*******************************************************************/

int x,y;

void main() {

  /*!npk x between 1 and 7 */
  x=x-2;
  
  if (x < 0)
	y=-x;	
  else
    y = x;

}
