`close.hkey` <-
function (con, ...)
{
    reg.finalizer(con, function(x){}, TRUE)
    class(con)<-NULL
}

