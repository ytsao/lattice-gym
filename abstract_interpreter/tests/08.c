/*******************************************************************/
/* Cas d'etudes pour le projet du cours d'interpretation abstraite */
/* Ecrit par Olivier Bouissou (olivier.bouissou@cea.fr)            */
/* Le but de ces cas d'etudes est de vous permettre de tester      */
/* votre projet sur des exemples de programmes contenant chacun    */
/* une difficulte que vous devriez rencontrer.                     */
/*******************************************************************/
/* Branchement conditionnel 3.                                     */
/* On cherche ici a tester les fonctions de transfert liees aux    */
/* test abstraits.                                                 */
/*******************************************************************/



int x,y,z;

void main() {

  x = 0;
  if (y != 0)
    z = 1;
  else
    z = -1;

  if (z == 0)
    x = 1;
}
