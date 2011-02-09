<?php

/*
 * Description: the code uses 2 templates named "index.tpl" and "results.tpl"
 * "results.tpl" is included in "index.tpl", though the inclusion is not carried
 * out via standard template-inclusion but via "rendering" having output-buffering enabled.
 * This allows post-processing of the results body ($results_body)
 *
 * To disable the debugging-popup that shows loading-time & template variables, 
 * set $tpl->debugging = false;
 *
 * Test with:
 *
 * www.wampserver.com
 * it's a pre-packaged solution e.g. for windows. 
 * there is also LAMP (for linux) W/M/L
 * put the files under /www 
 *
 *
 * Remember to change $DB_FILE to point to your urlbase.sql
 *
 */

error_reporting(E_ALL);

$DB_FILE = "sqlite:\arado\urlbase.sql";

include('lib/Smarty.class.php');

// SMARTY CONFIG 
$tpl = new smarty();
$tpl->template_dir = 'templates/normal';
$tpl->compile_dir = 'templates/compiled';
$tpl->cache_dir = 'templates/cache';
$tpl->config_dir = 'templates/configs';
$tpl->compile_check = false;
$tpl->caching = 0;
$tpl->force_compile = true;
$tpl->debugging = false;


// ACTUAL CODE FOLLOWS
if($_GET['q']){
    try {
        $db = new PDO ($DB_FILE);
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage();
    }
    if ($db) {
        //FIXME: adjust the query
        $q = @$db->query('SELECT hashid, url, description FROM urltable WHERE url LIKE "%'.
                         $_GET['q'].
                         '%" or description LIKE "%'.$_GET['q'].'%" LIMIT 10');
        if ($q) {
            $r = $q->fetchAll();
            //The following 3 lines render the template inside $results_body
            /* ob_start();
            $tpl->assign('RESULTS_BODY', $r); //FIXME: loop through the resultset & fix the output format
            $tpl->display('results.tpl');
            $results_body = ob_get_clean()."\r\n";
            
            //Post-processing and assignment of $results_body
            $results_body = strip_tags($results_body);
            //$tpl->assign('RESULTS', rawurlencode($results_body));
            */
            $tpl->assign('RESULTS', $r);
        }
        $tpl->display('index.tpl');
    } else {
        die($err);
    }
} else {
    $tpl->display('index.tpl');
}

?>