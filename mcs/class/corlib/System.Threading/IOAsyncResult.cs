// System.Threading.IOAsyncResult.cs
//
// Authors:
//	Ludovic Henry <ludovic@xamarin.com>
//
// Copyright (C) 2015 Xamarin, Inc. (https://www.xamarin.com)
//
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

/*
 * IAsyncResult implementation used by the IO threadpool.
 *
 * See :
 *  - metadata/io-selector.c
 */

using System.Runtime.InteropServices;
using System.Runtime.Remoting.Messaging;

namespace System.Threading
{
	internal enum IOOperation : int
	{
		/* Same enum in the runtime. Keep this in sync with
		 * MonoIOOperation in metadata/io-selector.c */

		Undefined = 0,
		In        = 1,
		Out       = 2,
	}

	[StructLayout (LayoutKind.Sequential)]
	internal abstract class IOAsyncResult : IAsyncResult, IThreadPoolWorkItem
	{
		/* Same fields in the runtime. Keep this in sync with
		 * MonoIOAsyncResult in metadata/io-selector.c */

		protected object state;
		protected ManualResetEvent wait_handle;
		protected bool completed_synchronously;
		protected bool completed;

		protected internal IntPtr handle;
		protected internal AsyncResult async_result;
		protected internal AsyncCallback async_callback;
		protected internal IOOperation operation;

		public object AsyncState
		{
			get { return state; }
		}

		public WaitHandle AsyncWaitHandle
		{
			get {
				lock (this) {
					if (wait_handle == null)
						wait_handle = new ManualResetEvent (completed);
					return wait_handle;
				}
			}
		}

		public bool CompletedSynchronously
		{
			get { return completed_synchronously; }
		}

		public bool IsCompleted
		{
			get {
				return completed;
			}
			set {
				completed = value;
				lock (this) {
					if (completed && wait_handle != null)
						wait_handle.Set ();
				}
			}
		}

		void IThreadPoolWorkItem.ExecuteWorkItem ()
		{
			Invoke ();
		}

		void IThreadPoolWorkItem.MarkAborted (ThreadAbortException tae)
		{
		}

		protected abstract void Invoke ();
	}
}
