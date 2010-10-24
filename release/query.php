<?php
/* this file should go in 
   arado/query.php 
   so that it can be reached as
   http://your.server.net/arado/query.php
*/
 
header ("Content-Type: text/xml");
header ("Connection: close");
header ("X-Data-Type: ADDR");
readfile ("addr-list.xml");
echo "\r\n";
?>
