<?php /* Smarty version 2.6.16, created on 2011-02-09 19:32:36
         compiled from index.tpl */ ?>
<?php echo '
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title>ARADO | empowering common people</title>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<meta name="description" content="The fastest way to search all the popular URLs on the web" />
<meta name="keywords" content="" />
<link href="styles.css" rel="stylesheet" type="text/css" />
<script type="text/javascript" src="mootools.js"></script>
</head>
'; ?>


<body>
    <div style="margin: 0px; overflow: hidden; width: 100%;">
        <div id="header" style="visibility: visible; opacity: 1;">
            <img src="arado_white.png" style="float: left;" />
            <div style="margin-top: 10%; margin-right: 10%;">
            <form name="f" method="get" action="">
            <label for="search" style="float: left; margin-top: 7px; margin-right: 20px;">
                Web Search: 
            </label>
            <input id="search" style="float: left; width: 50%; margin-right: 20px;" name="q" type="text" /> 
            <input type="submit" value="Search" style="float: left; margin-top: 7px;" />
            </form>
            </div>
        </div>
    </div>
<?php if ($_GET['q']): ?>  
    <div style="margin-top: 3%; margin-left: 3%;">
        <?php $_from = $this->_tpl_vars['RESULTS']; if (!is_array($_from) && !is_object($_from)) { settype($_from, 'array'); }if (count($_from)):
    foreach ($_from as $this->_tpl_vars['row']):
?>
            <span style="display: block; float: left; margin-top: 30px; clear: both;">
                <a style="display: block; float: left; height: 30px;" href="<?php echo $this->_tpl_vars['row'][1]; ?>
"><?php echo $this->_tpl_vars['row'][1]; ?>
</a>
                <br style="clear: both;" />
                <span style="clear: both;"><?php echo $this->_tpl_vars['row'][2]; ?>
 - <?php echo $this->_tpl_vars['row'][0]; ?>
</span>
            </span>
        <?php endforeach; endif; unset($_from); ?>    
    </div>
<?php endif; ?> 
<p><?php echo $this->_tpl_vars['FOOTER']; ?>
</p>
</body>
</html>