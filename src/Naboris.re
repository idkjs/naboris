module Server = Server;
module Req = Req;
module Res = Res;
module Method = Method;
module Router = Router;
module QueryMap = Query.QueryMap;
module MimeTypes = MimeTypes;
module Session = Session;
module SessionManager = SessionManager;
module Cookie = Cookie;
module ServerConfig = ServerConfig;
module Route = Route;

open Lwt.Infix;

let listen =
    (
      ~inetAddr=Unix.inet_addr_any,
      port,
      serverConfig: ServerConfig.t('sessionData),
    ) => {
  let listenAddress = Unix.(ADDR_INET(inetAddr, port));
  let connectionHandler = Server.buildConnectionHandler(serverConfig);

  Lwt.async(() =>
    Lwt_io.establish_server_with_client_socket(
      listenAddress,
      connectionHandler,
    )
    >>= (
      _server => {
        serverConfig.onListen();
        Lwt.return_unit;
      }
    )
  );

  Lwt.wait();
};

let listenAndWaitForever =
    (
      ~inetAddr=Unix.inet_addr_any,
      port,
      serverConfig: ServerConfig.t('sessionData),
    ) => {
  let (forever, _) = listen(~inetAddr, port, serverConfig)
  forever;
};