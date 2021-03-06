/*
 * eID Digital Signature Service Project.
 * Copyright (C) 2009-2010 FedICT.
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

package be.fedict.eid.dss.spi;

/**
 * Enumeration on the signature status.
 * 
 * @author Frank Cornelis
 * 
 */
public enum SignatureStatus {

	OK("OK"), USER_CANCELLED("USER_CANCELLED"), CERTIFICATE_INVALID(
			"CERTIFICATE_INVALID"), FILE_FORMAT("FILE_FORMAT"), FILE_SIZE(
			"FILE_SIZE");

	private final String status;

	private SignatureStatus(String status) {
		this.status = status;
	}

	public String getStatus() {
		return this.status;
	}
}
