`[.hkey` <-
function (x, indexes = hkey.clean.indexes(names(x))) 
{
    structure(lapply(indexes, function(i) x[[i]]), names = indexes)
}
