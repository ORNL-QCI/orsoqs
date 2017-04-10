%module orsoqs
%{
	#include "client.h"
	#include "socket.h"
	#include "msg.h"
	#include "liborsoqs.h"
%}

/* The following are callback processing functions for specific target languages. */
%{
	#ifdef SWIGPYTHON
	/**
	 * \brief The glue function to call a Python function from C.
	 * 
	 * The prototype must match up with what the callback defined in C expects.
	 * 
	 * \param	data		The input data buffer.
	 * \param	len			Length of the input data buffer.
	 * \param	out			Pointer to the output data buffer.
	 * \param	pycb		Pointer to the Python callable that is our callback.
	 * 
	 * \returns The number of bytes returned in the out buffer.
	 */
	static int64_t pycb_reqproc(const char* const data,
			int64_t len,
			char** out,
			const void *pycb) {
		int rc;
		PyGILState_STATE gstate;
		PyObject *arglist;
		PyObject *result;
		char* tbuf;
		int64_t outl;
		
		outl = 0;
		gstate = PyGILState_Ensure();
		
		/* Build our arguments to the Python callable. */
		arglist = Py_BuildValue("(s#)", data, len);
		
		/* This actually calls the callable callback. */
		result = PyObject_CallObject((PyObject*)pycb, arglist);
		
		Py_DECREF(arglist);
		
		if(PyErr_Occurred()) {
			/* If the callback raised an exception, we need to explicitly print it. */
			PyErr_Print();
			
			if(result != NULL) {
				Py_XDECREF(result);
			}
		} else if(result) {
			/* Return the internal buffer of the Python string as a C string. */
			rc = PyString_AsStringAndSize(result, &tbuf, &outl);
			
			Py_XDECREF(result);
			
			if(rc == 0 && outl != 0) {
				*out = malloc(outl);
				memcpy(*out, tbuf, outl);
			}
		}
		
		PyGILState_Release(gstate);
		return outl;
	}
	#endif
%}

/* These functions are internal and won't be exposed in the target language. */
%ignore _or_socket_proc_rdtimeo;
%ignore _or_socket_proc_wttimeo;
%ignore _or_socket_proc_t;
%ignore _or_wait_proc;
%ignore _or_reply_proc;
%ignore _or_zfree;

/* Rename functions that become bound in target language for succinctness. */
%rename or_create_client create_client;
%rename or_delete_client delete_client;
%rename or_connect connect;
%rename or_disconnect disconnect;
%rename or_create_msg create_msg;
%rename or_delete_msg delete_msg;
%rename or_send_msg send_msg;

%include "client.h"
%include "socket.h"
%include "msg.h"
%include "liborsoqs.h"

%extend or_socket_t {
	#ifdef SWIGPYTHON
	void set_proc_callback(PyObject * pyfunc) {
		int ret;
		PyGILState_STATE gstate;
		
		gstate = PyGILState_Ensure();
		
		if(!PyCallable_Check(pyfunc)) {
			PyErr_SetString(PyExc_TypeError,
					"set_proc_callback() needs a callable object!");
			PyErr_Print();
		} else {
			ret = set_socket_proc_cb(self, pycb_reqproc, (void*)pyfunc);
			
			if(ret != 0) {
				PyErr_Format(PyExc_RuntimeError,
						"set_socket_proc_cb() returned error code %d, %s",
						ret,
						or_error_str(-2));
				PyErr_Print();
			} else {
				Py_INCREF(pyfunc);
			}
		}
		
		PyGILState_Release(gstate);
	}
	#else
	#	error "Customize or_socket_t::set_proc_callback() for this SWIG language."
	#endif
}

%extend or_msg_t {
	#ifdef SWIGPYTHON
	PyObject* str() {
		/* Py_ssize_t provides us with 64-bit support. */
		return PyString_FromStringAndSize(self->data, (Py_ssize_t)self->size);
	}
	#else
	#	error "Customize or_msg_t::str() for this SWIG language."
	#endif
}

/* The following allows the target language to garbage collect with our functions. */
%newobject create_client;
%delobject delete_client;
%newobject connect;
%delobject disconnect;
%newobject create_msg;
%delobject delete_msg;
%newobject send_msg;
