{literal}
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
{/literal}

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
{if $smarty.get.q}  
    <div style="margin-top: 3%; margin-left: 3%;">
        {foreach from=$RESULTS item=row}
            <span style="display: block; float: left; margin-top: 30px; clear: both;">
                <a style="display: block; float: left; height: 30px;" href="{$row[1]}">{$row[1]}</a>
                <br style="clear: both;" />
                <span style="clear: both;">{$row[2]} - {$row[0]}</span>
            </span>
        {/foreach}    
    </div>
{/if} 
<p>{$FOOTER}</p>
</body>
</html>
