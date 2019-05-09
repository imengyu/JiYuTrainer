#include "stdafx.h"
#include "NetUtils.h"

// reply of the requery
UPEXPORT_CFUNC(size_t) req_reply(void *ptr, size_t size, size_t nmemb, void *stream)
{
	string *str = (string*)stream;
	(*str).append((char*)ptr, size*nmemb);
	return size * nmemb;
}

// http GET
UPEXPORT_CFUNC(CURLcode) curl_get_req(const string &url, string &response)
{
	// init curl
	CURL *curl = curl_easy_init();
	// res code
	CURLcode res;
	if (curl)
	{
		// set params
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // url
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); // if want to use https
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_HEADER, 0);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3); // set transport and time out time
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
		// start req
		res = curl_easy_perform(curl);
	}
	// release curl
	curl_easy_cleanup(curl);
	return res;
}

// http POST
UPEXPORT_CFUNC(CURLcode) curl_post_req(const string &url, const string &postParams, string &response)
{
	// init curl
	CURL *curl = curl_easy_init();
	// res code
	CURLcode res;
	if (curl)
	{
		// set params
		curl_easy_setopt(curl, CURLOPT_POST, 1); // post req
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // url
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postParams.c_str()); // params
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); // if want to use https
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_HEADER, 0);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
		// start req
		res = curl_easy_perform(curl);
	}
	// release curl
	curl_easy_cleanup(curl);
	return res;
}