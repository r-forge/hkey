`hkey.clean.indexes` <-
function (i) 
sub(paste("\\Q", hkey.subkey.suffix, "\\E$", sep = ""), "", 
    i, perl = TRUE)
