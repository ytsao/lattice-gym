/*******************************************************************/
/* Cas d'etudes pour le projet du cours d'interpretation abstraite */
/* Ecrit par Olivier Bouissou (olivier.bouissou@cea.fr)            */
/* Le but de ces cas d'etudes est de vous permettre de tester      */
/* votre projet sur des exemples de programmes contenant chacun    */
/* une difficulte que vous devriez rencontrer.                     */
/*******************************************************************/
/* Relationnel 2.                                                  */
/* On cherche ici a utiliser des domaines relationnels pour obtenir*/
/* des invariants plus precis.                                     */
/*******************************************************************/

int i, j;

void main() {

  i = 150;
  j = 175;
  while (j>=100) {
    i++;
    if (j<=i) {
      i = i-1;
      j = j-2;
    }
  }
}
