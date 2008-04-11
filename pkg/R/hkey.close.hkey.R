`hkey.close.hkey` <-
function (x) 
{
    invisible(.Call("close_node", x))
}
