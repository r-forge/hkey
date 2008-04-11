`$<-.hkey` <-
function (x, i, value) 
{
    x[[as.character(substitute(i))]] <- value
    hkey.fillin.hkey(x)
}
