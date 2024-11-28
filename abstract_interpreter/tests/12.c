/*******************************************************************/
/* Cas d'etudes pour le projet du cours d'interpretation abstraite */
/* Ecrit par Olivier Bouissou (olivier.bouissou@cea.fr)            */
/* Le but de ces cas d'etudes est de vous permettre de tester      */
/* votre projet sur des exemples de programmes contenant chacun    */
/* une difficulte que vous devriez rencontrer.                     */
/*******************************************************************/
/* Boucle 1.                                                       */
/* On cherche ici à tester l'itérateur et le calcul de point fixe. */
/* Dans ces exemples, des techniques de widening intelligentes     */
/* seront nécessaires pour obtenir des résultats précis.           */
/*******************************************************************/

int i, x, y, n;

void main()
{

  x = 1;
  y = 1;
  /*!npk n between 10 and 40 */

  while (i<=n)
  {
    y = x - 2*y;
    x = -x;
    i++;
  }

}
