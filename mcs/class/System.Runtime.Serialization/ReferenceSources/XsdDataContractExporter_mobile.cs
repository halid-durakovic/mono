//
// XsdDataContractExporter_mobile.cs
//
// Authors:
//	Alexander Köplinger <alexander.koeplinger@xamarin.com>
//
// Copyright (C) 2016 Xamarin Inc (http://www.xamarin.com)
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

using System;
using System.Collections.Generic;
using System.Reflection;
using System.Xml;
using System.Xml.Schema;

namespace System.Runtime.Serialization
{
	public class XsdDataContractExporter
	{
		public XsdDataContractExporter () { throw new NotImplementedException (); }
		public XsdDataContractExporter (XmlSchemaSet schemas) { throw new NotImplementedException (); }
		public ExportOptions Options { get { throw new NotImplementedException (); } set { throw new NotImplementedException (); } }
		public XmlSchemaSet Schemas { get { throw new NotImplementedException (); } }
		public bool CanExport (ICollection<Assembly> assemblies) { throw new NotImplementedException ();}
		public bool CanExport (ICollection<Type> types) { throw new NotImplementedException (); }
		public bool CanExport (Type type) { throw new NotImplementedException (); }
		public void Export (ICollection<Assembly> assemblies) { throw new NotImplementedException (); }
		public void Export (ICollection<Type> types) { throw new NotImplementedException (); }
		public void Export (Type type) { throw new NotImplementedException (); }
		public XmlQualifiedName GetRootElementName (Type type) { throw new NotImplementedException (); }
		public XmlSchemaType GetSchemaType (Type type) { throw new NotImplementedException (); }
		public XmlQualifiedName GetSchemaTypeName (Type type) { throw new NotImplementedException (); }
	}
}

