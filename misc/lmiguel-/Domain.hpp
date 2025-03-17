/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Domain.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmiguel- <lmiguel-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 15:47:20 by lmiguel-          #+#    #+#             */
/*   Updated: 2025/03/17 16:42:32 by lmiguel-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <Parser.hpp>

class Domain{

	public:

	Domain ( void );
	~Domain ( void );

	void	setDomainPort (int port);
	void	setDomainName (std::string name);

	/* SETUP ERROR CODE FUNCTIONS */

	private:

	int			_domainPort;
	std::string	_domainName;
	
	/* FIGURE OUT ERROR CODE STRUCTURING */
};

#endif