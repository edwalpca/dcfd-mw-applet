/*
 * eID Trust Service Project.
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

package be.fedict.trust.xkms2;

import java.lang.reflect.Field;

import javax.ejb.EJB;
import javax.servlet.ServletContext;
import javax.xml.ws.handler.MessageContext;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import be.fedict.trust.service.TrustService;

import com.sun.xml.ws.api.message.Packet;
import com.sun.xml.ws.server.AbstractMultiInstanceResolver;

/**
 * JAX-WS RI Instance Resolver implementation to inject services into JAX-WS
 * endpoints.
 * 
 * @author Frank Cornelis
 * 
 * @param <T>
 */
public class ServiceConsumerInstanceResolver<T> extends
		AbstractMultiInstanceResolver<T> {

	private static final Log LOG = LogFactory
			.getLog(ServiceConsumerInstanceResolver.class);

	public ServiceConsumerInstanceResolver(Class<T> clazz) {
		super(clazz);
	}

	@Override
	public T resolve(Packet request) {
		T endpoint = create();

		ServletContext servletContext = (ServletContext) request
				.get(MessageContext.SERVLET_CONTEXT);
		TrustService trustService = ServiceConsumerServletContextListener
				.getTrustService(servletContext);

		injectServices(endpoint, trustService);

		return endpoint;
	}

	private void injectServices(T endpoint, TrustService trustService) {
		LOG.debug("injecting services into JAX-WS endpoint...");
		Field[] fields = endpoint.getClass().getDeclaredFields();
		for (Field field : fields) {
			EJB ejbAnnotation = field.getAnnotation(EJB.class);
			if (null == ejbAnnotation) {
				continue;
			}
			if (field.getType().equals(TrustService.class)) {
				field.setAccessible(true);
				try {
					field.set(endpoint, trustService);
				} catch (Exception e) {
					throw new RuntimeException("injection error: "
							+ e.getMessage(), e);
				}
			}
		}
	}
}
