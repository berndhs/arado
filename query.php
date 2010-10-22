<?php
/* this file should go in 
   arado/query.php 
   so that it can be reached as
   http://your.server.net/arado/query.php
   the readfile - next to it in the same path - is nameed: addresses.xml
*/
 
header ("Content-Type: text/xml");
header ("Connection: close");
header ("X-Data-Type: ADDR");
readfile ("addresses.xml");
echo "\r\n";
?>
