/*
 * @progname       ancestors2.ll
 * @version        2.0
 * @author         Wetmore, Cliff Manis
 * @category       
 * @output         Text
 * @description    
 *
 *   It will produce a report of all ancestors of a person, with
 *   sorted names as output, birth and death dates.
 *
 *   ancestors2
 *
 *   Initial Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   Modifications by Cliff Manis, this is a modification of the
 *   report "ancestors1".
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1990,
 *   and it has been modified many times since.
 *
 *   It will produce a report of all ancestors of a person, with
 *   sorted names as output, birth and death dates.
 *
 *   It will produce ASCII file output.
 *
 */

proc main ()
{
        indiset(a)
        monthformat(4)
        indiset(b)
        getindi(i)
        addtoset(a, i, 0)
        set(b,ancestorset(a))
        namesort(b)
        "ANCESTORS OF -- " upper(name(i)) "    (" key(i) ") " nl() nl()
        forindiset(b, i, x, n) {
                col(1) fullname(i,1,0,36)
                col(38) key(i)
                col(49) stddate(birth(i))
                col(64) stddate(death(i)) nl()
        }
}

/* Sample output of report

ANCESTORS OF -- ALDA CLIFFORD MANIS    (107)

BARTH, Johann Ludwig                 2243              1730    23 Jun 1770
BIRD, Frances Amanda                 377         8 Feb 1845    26 Dec 1898
BIRD, Jacob                          783               1760
BIRD, John                           551        11 Oct 1795     3 Jul 1889
BIRD, Wife of Jacob                  784
BOWERS, Anderson                     20                1803
BOWERS, James                        52
BOWERS, Martha A.                    9          14 Apr 1829    22 Jul 1899
BOYD, Mary                           55                1772
BRADSHAW, Jennet                     9723       21 May 1772    24 Jan 1824
BRADSHAW, John F.                    10033         Mar 1743    30 Sep 1818
CANTER, Cordelia "Corda" F.          39          7 Dec 1869    18 Apr 1960
CANTER, Henry B.                     162               1820
CANTER, James H.                     80                1847    27 Dec 1937
CARROLL, Joseph                      12109
CARROLL, Sarah                       9967
CLENDENIN, Agnes "Annie"             10034         May 1748       Aug 1823
CLENDENIN, Isabella                  10021
CLENDENIN, John                      10133             1704           1797
CORBETT, James                       896
CORBETT, John                        1122
CORBETT, John Williams               607
CORBETT, Mary Jane                   386         9 Oct 1843     2 Nov 1918
COWAN, Christopher Columbus          54                1765
COWAN, Lurina Viney "Vina"           21                1808
COWAN, Samuel                        699
COWAN, William                       2250
CROCKETT, Mary                       771               1780
EUDAILY, Betsy                       608
FOSTER, Martha "Patsy"               6906              1810           1870
FOSTER, Thomas                       6902              1780
FOSTER, Wife of Thomas               6903
FRANCIS, David                       770               1769           1850
FRANCIS, Edward                      965               1745           1800
FRANCIS, Mary Elizabeth              549        15     1810
FRANKLIN, Nancy                      9703
GRESHAMS, Polly                      897
HOUSTON, Janet "Jean"                10134                            1797
HOUSTON, John                        13239                      6 Dec 1769
MANES, Fuller Ruben                  45         19 Nov 1902    20 Jun 1980
MANES, Samuel P.                     1                 1780       Jan 1831
MANES, William Bowers                16          6 Jan 1868     5 May 1933
MANES, William Thomas                8          26 Nov 1828     2 Mar 1907
MANESS, John                         3643              1770
MANIS, Amos                          548               1805           1840
MANIS, Edith Alberta                 105         8 Apr 1914    18 Jun 1992
MANIS, Thomas D.A.F.S.               376         1 Feb 1839    25 Sep 1919
MANIS, William Loyd                  220         5 Sep 1872    15 Mar 1946
MCELWEE, Jane                        10123
MCMURTRY, Anna                       2240                       9 Feb 1849
NEWMAN, Aaron                        9632       18 Jan 1802    24 Jul 1884
NEWMAN, John                         9702       11 Dec 1782     8 Oct 1865
NEWMAN, John Franklin                9617        4 May 1830    18 Sep 1921
NEWMAN, Jonathan                     9966       25 Dec 1730    17 May 1817
NEWMAN, Lillie Caroline "Carolyn"    221        13 Jun 1881    29 Sep 1949
RANKIN, Alexander                    10178
RANKIN, John                         10122             1690           1749
RANKIN, Sinea                        9633        7 May 1806    23 Mar 1833
RANKIN, Thomas                       10020             1724           1812
RANKIN, Thomas B.                    9722        3 Mar 1764     3 Aug 1821
RANKIN, William                      10165
REED, Elizabeth                      1123
SHRADER, Elizabeth                   552               1799    19 Feb 1885
SHRADER, G. Christopher              786               1776
STEWART, George                      13237
STEWART, Martha                      13238
WEBB, Jesse                          2239              1766    25 Mar 1848
WEBB, Mary                           787               1789
WHITEHORN, James                     2824                             1860
WHITEHORN, Martha Marie              81         22 Dec 1846

 end of report sample */
