type t;

/**
 Creates a default response record with empty headers and a 200 status.
 */
let default: unit => t;

/**
 Creates new response from input response with status of [int].
 */
let status: (int, t) => t;

/**
 Sends response [t] with body [string].
 Adding headers [Content-type: application/json] and [Content-length]

 {e This function will end the http request/response lifecycle.}
 */
let json: (Req.t('sessionData), string, t) => Lwt.t(unit);

/**
 Sends response [t] with body [string].
 Adding headers [Content-type: text/html] and [Content-length]

 {e This function will end the http request/response lifecycle.}
 */
let html: (Req.t('sessionData), string, t) => Lwt.t(unit);

/**
 Sends response [t] with body [string].
 Adding headers [Content-type: text/plain] and [Content-length]

 {e This function will end the http request/response lifecycle.}
 */
let text: (Req.t('sessionData), string, t) => Lwt.t(unit);

/**
 Sends response [t] with body [string].

 {e This function will not add any headers other than [Content-length] with the length of [string].}
 {e This function will end the http request/response lifecycle.}
 */
let raw: (Req.t('sessionData), string, t) => Lwt.t(unit);

/**
 Creates new response from [t] with header [(string, string)] added.
 */
let addHeader: ((string, string), t) => t;

/**
 Opens file starting at path [string] and following [list(string)].
 Sets [Content-type] header based on file extension.  If type cannot be inferred [text/plain] is used.
 Sets [Content-length] header with the size of the file in bytes.
 Responds with [404] if file does not exist.

 {e This function will end the http request/response lifecycle.}
 */
let static: (string, list(string), Req.t('sessionData), t) => Lwt.t(unit);

/**
 Sets [Location] header to [string] and responds with [302].
 Redirecting client to [string].
 */
let redirect: (string, Req.t('sessionData), t) => Lwt.t(unit);


/**
 Report an error [exn] to Httpaf.
 */
let reportError: (Req.t('sessionData), exn) => unit;

/**
 Adds [Set-Cookie] header to response [t] with sessionId [string] as a value.
 Uses ["nab.sid"] as the key for parsing the cookie later.
 Uses 30 days [Max-Age] for expiration.

 {e These will be configurable in future versions.}
 */
let setSessionCookies: (string, t) => t;
