`$.hkey` <-
function (x, name) 
{
    name <- as.character(substitute(name))
    x[[name]]
}
