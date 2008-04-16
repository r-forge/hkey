
<!-- This is the project specific website template -->
<!-- It can be changed as liked or replaced by other content -->

<?php

$domain=ereg_replace('[^\.]*\.(.*)$','\1',$_SERVER['HTTP_HOST']);
$group_name=ereg_replace('([^\.]*)\..*$','\1',$_SERVER['HTTP_HOST']);
$themeroot='http://r-forge.r-project.org/themes/rforge/';

echo '<?xml version="1.0" encoding="UTF-8"?>';
?>
<!DOCTYPE html
	PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en   ">

  <head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
	<title><?php echo $group_name; ?></title>
	<link href="<?php echo $themeroot; ?>styles/estilo1.css" rel="stylesheet" type="text/css" />
  </head>

<body>

<! --- R-Forge Logo --- >
<table border="0" width="100%" cellspacing="0" cellpadding="0">
<tr><td>
<a href="/"><img src="<?php echo $themeroot; ?>/images/logo.png" border="0" alt="R-Forge Logo" /> </a> </td> </tr>
</table>


<!-- get project title  -->
<!-- own website starts here, the following may be changed as you like -->

<?php if ($handle=fopen('http://'.$domain.'/export/projtitl.php?group_name='.$group_name,'r')){
$contents = '';
while (!feof($handle)) {
	$contents .= fread($handle, 8192);
}
fclose($handle);
echo $contents; } ?>

<!-- end of project description -->

<p>If you love the Windows registry, you'll like <strong>hkey</strong>.</p>

<pre>
<b>
> library(hkey)
> hkey()
</b>
Win32 registry node: HKEY_LOCAL_MACHINE
Use names() to list subkeys and data, $, [], [[]] to extract them.

<b>
> names(hkey())
</b>
[1] "HARDWARE/" "SAM/"      "SECURITY/" "SOFTWARE/" "SYSTEM/"  

<b>
> x <- hkey("HKEY_LOCAL_MACHINE/SOFTWARE")
> names(x)
</b>
 [1] "781/"                          "Adi/"                         
 [3] "Adobe/"                        "Agere/"                       
 [5] "Ahead/"                        "Alps/"                        
 [7] "ALPS Electric Co., Ltd./"      "Analog Devices/"              
 [9] "Andrea Electronics/"           "Ariad/"                       
...
[47] "Skype/"                        "Staccato/"                    
[49] "Sun Microsystems/"             "TOSHIBA/"                     
[51] "Wilson WindowWare/"            "Windows/"                     
[53] "Windows 3.1 Migration Status/" "Wireless/"                    
<b>
> x <- x$"R-core"
> names(x)
</b>
[1] "R/"
<b>
> x <- x[["R"]]
> x[]
</b>
$`2.2.1`
Win32 registry node: HKEY_LOCAL_MACHINE/SOFTWARE/R-core/R/2.2.1
Use names() to list subkeys and data, $, [], [[]] to extract them.
...
$`2.4.1`
Win32 registry node: HKEY_LOCAL_MACHINE/SOFTWARE/R-core/R/2.4.1
Use names() to list subkeys and data, $, [], [[]] to extract them.

$`Current Version`
[1] "2.4.1"
attr(,"class")
[1] "REG_SZ"

$InstallPath
[1] "C:\\Program Files\\R\\R-2.4.1"
attr(,"class")
[1] "REG_SZ"

> as.list(x)
$`2.2.1`
$`2.2.1`$InstallPath
[1] "C:\\Program Files\\R\\R-2.2.1"

...

$`2.4.1`
$`2.4.1`$InstallPath
[1] "C:\\Program Files\\R\\R-2.4.1"


$`Current Version`
[1] "2.4.1"

$InstallPath
[1] "C:\\Program Files\\R\\R-2.4.1"

<b>
> x$new.value <- "This will be ignored by R, I hope"
> x$new.subnode <- list(a=12345, b=c("one","more","time"), d=as.raw(10:1), e=list(nesting=c(2,2)))
> as.list(x)
</b>
$`2.2.1`
$`2.2.1`$InstallPath
[1] "C:\\Program Files\\R\\R-2.2.1"

...

$new.subnode
$new.subnode$e
$new.subnode$e$nesting
[1] 2 2


$new.subnode$a
[1] 12345

$new.subnode$b
[1] "time" ""     ""    

$new.subnode$d
 [1] 0a 09 08 07 06 05 04 03 02 01


$`Current Version`
[1] "2.4.1"

$InstallPath
[1] "C:\\Program Files\\R\\R-2.4.1"

$new.value
[1] "This will be ignored by R, I hope"

<b>
> x[c("new.value", "new.subnode")] <- list(NULL, NULL)
> names(x)
</b>
[1] "2.2.1/"          "2.3.0/"          "2.3.1/"          "2.4.0/"          "2.4.1/"         
[7] "Current Version" "InstallPath"

</pre>

<p> The <strong>project summary page</strong> you can find <a href="http://<?php echo $domain; ?>/projects/<?php echo $group_name; ?>/"><strong>here</strong></a>. </p>

</body>
</html>
