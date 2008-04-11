`[<-.hkey` <-
function (x, i, value) 
{
    for (j in i) x[[j]] <- value[[j]]
    hkey.fillin.hkey(x)
}
