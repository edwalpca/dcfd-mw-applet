/*
 * eID Identity Provider Project.
 * Copyright (C) 2010-2012 FedICT.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version
 * 3.0 as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, see
 * http://www.gnu.org/licenses/.
 */

package be.fedict.eid.idp.protocol.ws_federation.wsfed;

import org.opensaml.common.impl.AbstractSAMLObjectBuilder;

public class SecurityTokenServiceBuilder extends
		AbstractSAMLObjectBuilder<SecurityTokenService> {

	@Override
	public SecurityTokenService buildObject() {
		return buildObject(SecurityTokenService.DEFAULT_ELEMENT_NAME);

	}

	@Override
	public SecurityTokenService buildObject(String namespaceURI,
			String localName, String namespacePrefix) {
		return new SecurityTokenServiceImpl(namespaceURI, localName,
				namespacePrefix);
	}
}
