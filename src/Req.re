type t('sessionData) = {
  requestDescriptor: Httpaf.Reqd.t,
  session: option(Session.t('sessionData)),
};

let reqd = req => req.requestDescriptor;

let getHeader = (headerKey, req) =>
  switch (Httpaf.Reqd.request(req.requestDescriptor)) {
  | {headers, _} => Httpaf.Headers.get(headers, headerKey)
  };

let getBody = ({requestDescriptor, _}) => {
  let body = Httpaf.Reqd.request_body(requestDescriptor);
  let (bodyStream, pushToBodyStream) = Lwt_stream.create();

  let rec on_read = (bigstr, ~off as _, ~len as _) => {
    let str = Bigstringaf.to_string(bigstr);
    pushToBodyStream(Some(str));
    Httpaf.Body.schedule_read(body, ~on_read, ~on_eof);
  }
  and on_eof = () => pushToBodyStream(None);

  Httpaf.Body.schedule_read(body, ~on_read, ~on_eof);

  Lwt_stream.fold((a, b) => a ++ b, bodyStream, "");
};

let fromReqd = reqd => {
  let defaultReq = {requestDescriptor: reqd, session: None};
  defaultReq;
};

let getSessionData = req => {
  switch (req.session) {
  | None => None
  | Some(session) => Some(Session.data(session))
  };
};

let setSessionData = (maybeSession, req) => {
  {...req, session: maybeSession};
};