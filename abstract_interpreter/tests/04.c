/*******************************************************************/
/* Cas d'études pour le projet du cours d'interprétation abstraite */
/* Ecrit par Olivier Bouissou (olivier.bouissou@cea.fr)            */
/* Le but de ces cas d'etudes est de vous permettre de tester      */
/* votre projet sur des exemples de programmes contenant chacun    */
/* une difficulte que vous devriez rencontrer.                     */
/*******************************************************************/
/* Une boucle pas si simple.                                       */
/* N'oubliez pas que sur la plupart des architectures, les entiers */
/* sont codes sur 32 bits                                          */
/*******************************************************************/

int x;
int i;

void main() 
{
  x = 2147483640;
  i = 1000;
  x = x+i;
}
